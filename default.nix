# Nix development environment
#
# nix-build -I "BPPpkgs=https://github.com/adevress/bbp-nixpkgs/archive/master.tar.gz" default.nix
# nix-shell -I "BPPpkgs=https://github.com/adevress/bbp-nixpkgs/archive/master.tar.gz"  default.nix
#
with import <BBPpkgs> {};
{

    morpho-tool = morpho-tool.overrideDerivation (oldAttr: rec {
      name = "morpho-tool-DEV_ENV";
      src = ./.;      
    });

}


