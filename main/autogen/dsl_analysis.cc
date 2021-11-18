#include "main/autogen/dsl_analysis.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "ast/treemap/treemap.h"
#include "common/formatting.h"
#include "main/autogen/crc_builder.h"

using namespace std;
namespace sorbet::autogen {

class DSLAnalysisWalk {
    UnorderedMap<vector<core::NameRef>, DSLInfo> dslInfo;
    vector<vector<core::NameRef>> nestingScopes;

    // Convert a symbol name into a fully qualified name
    vector<core::NameRef> symbolName(core::Context ctx, core::SymbolRef sym) {
        vector<core::NameRef> out;
        while (sym.exists() && sym != core::Symbols::root()) {
            out.emplace_back(sym.data(ctx)->name);
            sym = sym.data(ctx)->owner;
        }
        reverse(out.begin(), out.end());
        return out;
    }

    // Convert a constant literal into a fully qualified name
    vector<core::NameRef> constantName(core::Context ctx, ast::ConstantLit *cnst) {
        vector<core::NameRef> out;
        while (cnst != nullptr && cnst->original != nullptr) {
            auto &original = ast::cast_tree_nonnull<ast::UnresolvedConstantLit>(cnst->original);
            out.emplace_back(original.cnst);
            cnst = ast::cast_tree<ast::ConstantLit>(original.scope);
        }
        reverse(out.begin(), out.end());
        return out;
    }

public:
    DSLAnalysisWalk() {}

    ast::ExpressionPtr preTransformClassDef(core::Context ctx, ast::ExpressionPtr tree) {
        auto &original = ast::cast_tree_nonnull<ast::ClassDef>(tree);
        if (original.symbol.data(ctx)->owner == core::Symbols::PackageRegistry()) {
            // this is a package, so do not enter a definition for it
            return tree;
        }

        vector<vector<core::NameRef>> ancestors;
        for (auto &ancst : original.ancestors) {
            auto *cnst = ast::cast_tree<ast::ConstantLit>(ancst);
            if (cnst == nullptr || cnst->original == nullptr) {
                // ignore them if they're not statically-known ancestors (i.e. not constants)
                continue;
            }

            ancestors.emplace_back(symbolName(ctx, cnst->symbol));
        }

        const vector<core::NameRef> className = symbolName(ctx, original.symbol);
        nestingScopes.emplace_back(className);
        dslInfo.emplace(className, DSLInfo{{}, ancestors});

        return tree;
    }

    ast::ExpressionPtr postTransformClassDef(core::Context ctx, ast::ExpressionPtr tree) {
        if (nestingScopes.size() == 0) {
            // Not in any scope
            return tree;
        }

        nestingScopes.pop_back();

        return tree;
    }

    ast::ExpressionPtr preTransformSend(core::Context ctx, ast::ExpressionPtr tree) {
        if (nestingScopes.size() == 0) {
            // Not in any scope
            return tree;
        }

        auto *original = ast::cast_tree<ast::Send>(tree);
        auto &curScope = nestingScopes.back();
        if (original->fun.rawId() == core::Names::prop().rawId()) {
            auto *lit = ast::cast_tree<ast::Literal>(original->args.front());
            if (lit && lit->isSymbol(ctx)) {
                dslInfo[curScope].props.emplace_back(lit->asSymbol(ctx));
            }
        }
        return tree;
    }

    DSLAnalysisFile dslAnalysisFile() {
        DSLAnalysisFile out;
        out.dslInfo = std::move(dslInfo);
        return out;
    }
};

DSLAnalysisFile DSLAnalysis::generate(core::Context ctx, ast::ParsedFile tree, const CRCBuilder &crcBuilder) {
    DSLAnalysisWalk walk;
    ast::TreeMap::apply(ctx, walk, move(tree.tree));
    auto daf = walk.dslAnalysisFile();
    auto src = tree.file.data(ctx).source();
    daf.cksum = crcBuilder.crc32(src);
    return daf;
}

} // namespace sorbet::autogen
