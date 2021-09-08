# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL
# run_filecheck: OPT

# Verify that we do reify the block when it's invoked in a block, even inside begin.

def boo(array, &blk)
  array.each do |x|
    begin
      p x
      yield
    ensure
      p "in ensure"
    end
  end
end

# INITIAL-LABEL: define i64 @"func_Object#boo"
# INITIAL: call i64 @sorbet_getMethodBlockAsProc
# INITIAL{LITERAL}: }

# OPT-LABEL: define i64 @"func_Object#boo"
# OPT: call i64 @rb_block_proc
# OPT{LITERAL}: }

# INITIAL-LABEL: define internal i64 @"func_Object#boo$block_2"
# INITIAL-NOT: call i64 @sorbet_getMethodBlockAsProc
# INITIAL-NOT: call i64 @rb_block_proc
# INITIAL: @sorbet_i_send(%struct.FunctionInlineCache* @ic_call
# INITIAL-NOT: call i64 @sorbet_getMethodBlockAsProc
# INITIAL-NOT: call i64 @rb_block_proc
# INITIAL{LITERAL}: }

boo([1, 2]) do
  puts "boohey"
end

