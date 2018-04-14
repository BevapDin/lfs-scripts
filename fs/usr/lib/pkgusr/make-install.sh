#! /bin/bash

# Dies ist ein Wrapper-Skript, welches im großen und
# ganzen nur 'make install 1>log 2>err' ausführt.

# Das Error-Log (stderr) wird in $LOG_PATH als install.err gespeichert,
# das log (stdout) in $LOG_PATH/install.log.
# Standard für $LOG_PATH ist $HOME.
# ACHTUNG: bedingt durch die Benutzung von tee wird beides in install.log
# gespeichert.

# Alle Argumente werden an make weitergegeben (nach dem install).

# Optionen:
# -x ... sendet kein 'install' an make, sondern nur die weiteren
# Argumente des Skriptes.

# Beispiele:
# make-install.sh	führt 'make install' aus.
# make-install.sh -x	'make'
# make-install.sh opt	'make install opt'
# make-install.sh -x o	'make o'


function usage {
  echo $0 '[-i] [args]'
  echo 'executes `make install <arg>`'
  echo ' -i ... dont use install, use only args instead.'
  exit 1;
}

MAKE_CMD=install

if [ -n "$1" ] ; then
  case "$1" in
    -h|-\?|--help) usage ;;
    -i) MAKE_CMD="" ;;
  esac
fi

if [ -z "$LOG_PATH" ] ; then
  LOG_PATH="$HOME"
fi

if [ ! -d "$HOME/pkg" ] ; then
  mkdir "$HOME/pkg"
fi

echo Making "$MAKE_CMD" "$@"
make "$MAKE_CMD" "$@" 2>&1 | tee $LOG_PATH/install.log || exit $?

rm -r "$HOME/pkg" 2>/dev/null

exit 0



