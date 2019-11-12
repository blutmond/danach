#!/bin/bash
echo "#!/bin/bash" > /tmp/bootstrap-tmp.sh
.build/rules src/rules rules-dynamic >> /tmp/bootstrap-tmp.sh
.build/rules src/parser parser-dynamic >> /tmp/bootstrap-tmp.sh
# remove "bad lines"
cat /tmp/bootstrap-tmp.sh | grep -v "Success!" | grep -v "^.build/parser" | grep -v "^.build/lowering-spec-tool" > /tmp/bootstrap.sh
chmod +x /tmp/bootstrap.sh
rm /tmp/bootstrap-tmp.sh
mv /tmp/bootstrap.sh tools/bootstrapping.sh -f
