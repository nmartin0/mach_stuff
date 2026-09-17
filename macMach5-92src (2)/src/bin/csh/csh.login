# standard Mach .login
setenv TERM ""
eval "`machterm -csh`"
if ("$TERM" == '') then
  echo -n "term: "
  set i = $<
  if ("$i" == '') then
    setenv TERM "ansi"
  else
    setenv TERM "$i"
  endif
  unset i
  echo ""
endif
