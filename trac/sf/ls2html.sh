#!/bin/bash

cat > index.html << EOF
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>
<head>
<title>Directory Listing for $1</title>
</head>
<body>
<h1> Directory Listing for $1</h1>
<ul>
EOF

#for foo in `ls -g -G | awk -F '-- ' '{print $2}' ` ; do
for foo in `ls -1 *.gz` ; do
    size=`ls -sh ${foo} | cut -f1 -d' '`
    echo "<li><a href=\"${foo}\">${foo}</a>       ${size}</li>" >> index.html
done


cat >> index.html << EOF
</ul>
</body>
</html>
EOF

