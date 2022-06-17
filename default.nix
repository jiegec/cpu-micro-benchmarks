with import <nixpkgs> {};

stdenv.mkDerivation {
  name = "cpu-micro-benchmarks";
  version = "1.0";

  src = ./.;

  nativeBuildInputs = [
    meson
    ninja
  ];

  buildInputs = [
  ];
}

