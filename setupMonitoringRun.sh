rm /tmp/enableOracle
echo "0" > /tmp/enableMonitor
rm monitor*
find benchmark/net/ -name "*.js" -exec sed -i "s/dur: \[0.25\]/dur: \[15\]/g" {} \;
