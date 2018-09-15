with import <nixpkgs> {};

stdenv.mkDerivation {
  name = "rawlabot-devenv";
  buildInputs = [ walabot-sdk python3Packages.matplotlib ];
}
