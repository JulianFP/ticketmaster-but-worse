{ pkgs ? import <nixpkgs> { } }:
let
  myPythonPackages = ps: with ps; [
    flask
    flask-jwt-extended
    pyotp
    gunicorn
  ];
in 
pkgs.mkShell {
  buildInputs = with pkgs; [
    #dependencies for running the server
    (python3.withPackages myPythonPackages)

    #dependencies for building and running the client
    cmake
    openssl
    qt5.full

    #tools for solving the hacking challenge
    radare2
  ];
}
