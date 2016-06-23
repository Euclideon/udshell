export LD_LIBRARY_PATH=/opt/Euclideon/Shell/Qt
/opt/Euclideon/Shell/epshell "$@" &
sleep 2s
sudo gdbserver --attach host:2345 $!
