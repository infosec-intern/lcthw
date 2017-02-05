#!/bin/bash
EX="$1"

SKEL="#include <stdio.h>\n
\nint main(int argc, char *argv[])\n
{\n
  return 0;\n
}"

mkdir $EX
cat Makefile | sed -e "s/EX=/EX=$EX/" > $EX/Makefile
echo -e "#include <stdio.h>\n" > $EX/$EX.c
echo "int main(int argc, char *argv[])" >> $EX/$EX.c
echo "{" >> $EX/$EX.c
echo -e "\n" >> $EX/$EX.c
echo "  return 0;" >> $EX/$EX.c
echo "}" >> $EX/$EX.c

cp $EX/$EX.c $EX/broken.c
cp $EX/$EX.c $EX/extracredit.c
cp ./dbg.h $EX/dbg.h

