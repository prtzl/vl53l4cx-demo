{
    inputs = {
        nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
        jlink-pack.url = "github:prtzl/jlink-nix"; # jlink debugger support
        flake-utils.url = "github:numtide/flake-utils";
    };

    outputs = inputs:
    with inputs;
    flake-utils.lib.eachDefaultSystem (system:
        let
            pkgs = nixpkgs.legacyPackages.${system};
            stdenv = pkgs.stdenv;
            jlink = jlink-pack.defaultPackage.${system};
            firmware = pkgs.callPackage ./default.nix { };
            flash-script = pkgs.writeTextFile {
                name = "flash-script";
                text = ''
                    device ${firmware.device}
                    si 1
                    speed 4000
                    loadfile ${firmware}/bin/${firmware.name}.bin,0x08000000
                    r
                    g
                    qc
                '';
            };
            flash = pkgs.writeShellApplication {
                name = "flash";
                text = "JLinkExe -commanderscript ${flash-script}";
                runtimeInputs = [ jlink ];
            };
            flash-st = pkgs.writeShellApplication {
              name = "flash-st";
              text = "st-flash --reset write ${firmware}/bin/${firmware.name}.bin 0x08000000";
              runtimeInputs = [ pkgs.stlink ];
            };
        in {
            inherit firmware flash flash-st;
            defaultPackage = firmware;

            devShell = pkgs.mkShell {
                nativeBuildInputs = (firmware.nativeBuildInputs or []) ++ [ pkgs.clang-tools jlink pkgs.stlink ];
                LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath [ pkgs.llvmPackages_11.llvm ];
            };
    });
}
