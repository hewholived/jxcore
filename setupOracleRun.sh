echo "0" > /tmp/enableOracle
rm /tmp/enableMonitor
find benchmark/ -name "*.js" -exec sed -i "s/dur: \[15\]/dur: \[1\]/g" {} \;
