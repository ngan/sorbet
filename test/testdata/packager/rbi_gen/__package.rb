# frozen_string_literal: true
# typed: strict
# enable-packager: true

class RBIGen < PackageSpec
    export RBIGen::Public
    export Test::RBIGen
    export_for_test RBIGen::Private::PrivateClassForTests
end
