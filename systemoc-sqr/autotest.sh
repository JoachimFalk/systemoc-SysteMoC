# -will be processed by BASH, shebang is inserted by automake-

# simple autotest for the SQR example. We just start the binary, make sure
# no output confuses caller, and return exit code
#./simulation-sqr &> /dev/null

# 
test_bin=$buildir/simulation-sqr

echo "$builddir"
echo "$srcdir"
