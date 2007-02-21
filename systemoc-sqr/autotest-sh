# -will be processed by BASH, shebang is inserted by make-

#
# simple autotest for the SQR example. We just start the binary with parameter
# and check the output. Some confusing debug output is send to nirvana.
#
# autovars like $srcdir are inserted by make.
#

#
golden_logs_dir=$srcdir/testlogs

logoutfile=autotest.log

#
test_bin=$builddir/simulation-sqr

# call binary
$test_bin 100 > $logoutfile 2> /dev/null

# make sure 'golden' logfile exist! i leave checks for clarity
diff $logoutfile $golden_logs_dir/100iter.log > /dev/null

if [ $? -gt 0 ]; then
  echo "SQR autotest: logfiles differ."
  test_failed=1
fi

# remove test logfile
rm -f $logoutfile

[[ -z "$test_failed" ]]
