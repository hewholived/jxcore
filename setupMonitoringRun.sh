rm /tmp/enableOracle
echo "0" > /tmp/enableMonitor
rm monitor*
find benchmark/ -name "*.js" -exec sed -i "s/dur: \[5\]/dur: \[15\]/g" {} \;
