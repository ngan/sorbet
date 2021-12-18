# frozen_string_literal: true
# typed: strict

class Project::Bar::Bar
  extend T::Sig
  sig {params(value: Integer).void}
  def initialize(value)
    @value = T.let(value, Integer)
  end

  class << self
    extend T::Sig
    sig {void}
    def method_on_singleton_class
    end
  end
end
