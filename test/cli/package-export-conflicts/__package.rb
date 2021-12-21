# typed: strict
# enable-packager: true

class MyPackage < PackageSpec
  export MyPackage::A::B
  export MyPackage::A::C
  export MyPackage::A

  export MyPackage::X
  export MyPackage::X::Y
  export MyPackage::X::Z
end
