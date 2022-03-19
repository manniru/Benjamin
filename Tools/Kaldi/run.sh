#mkdir bin
#ldd BaTool | awk 'NF == 4 { system("cp " $3 " bin") }'

LD_LIBRARY_PATH="/home/bijan/Project/Benjamin/Tools/bin" ./BaTool

