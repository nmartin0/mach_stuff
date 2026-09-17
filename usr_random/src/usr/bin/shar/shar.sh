#!/bin/sh
echo "#!/bin/sh"
for i in $*; do
  echo "a $i" 1>&2
  echo "echo \"x $i\" 1>&2"
  echo "cat >$i <<'@EOF@'"
  cat $i
  echo "@EOF@"
  echo ""
done
