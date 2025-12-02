# vscode cpp tools is proprietary and I don't think you can configure nix home manager to install it for you.
# sad

{
  description = "Nix devShell";

  inputs = {
    # You can swap this for a FlakeHub or Determinate pin if you want.
    nixpkgs.url = "https://flakehub.com/f/NixOS/nixpkgs/0";
  };

  outputs =
    { self, nixpkgs, ... }:
    let
      system = "aarch64-darwin";
      pkgs = import nixpkgs {
        inherit system;
        # optional: allowUnfree = true; if you want official MS VSCode instead of code-oss
        config.allowUnfree = true;
      };
    in
    {
      devShells.${system}.default = pkgs.mkShell {
        # Optional: environment variables to nudge VSCode to use a per-project config.
        # You can point these at .vscode in the repo if you want full per-project settings.
        shellHook = ''
          echo "[minecraft-go] Dev shell activated."
          echo "VS Code: $(code --version 2>/dev/null || echo 'code not found')"
        '';
      };
    };
}
