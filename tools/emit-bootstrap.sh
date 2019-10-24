#!/bin/bash
echo "#!/bin/bash" > /tmp/bootstrap-tmp.sh
.build/rules src/rules rules-dynamic >> /tmp/bootstrap-tmp.sh
.build/rules src/parser parser-dynamic >> /tmp/bootstrap-tmp.sh
.build/rules src/parser lowering-spec-tool-dynamic >> /tmp/bootstrap-tmp.sh
# remove "bad lines"
cat /tmp/bootstrap-tmp.sh | grep -v "Success!" | grep -v "^.build/parser" | grep -v "^.build/lowering-spec-tool" > /tmp/bootstrap.sh
chmod +x /tmp/bootstrap.sh
rm /tmp/bootstrap-tmp.sh
echo "mv .build/parser-dynamic .build/parser" >> /tmp/bootstrap.sh
echo "mv .build/lowering-spec-tool-dynamic .build/lowering-spec-tool" >> /tmp/bootstrap.sh
mv /tmp/bootstrap.sh tools/bootstrapping.sh -f
