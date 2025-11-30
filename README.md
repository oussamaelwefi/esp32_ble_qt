<h1> Sensors data reading using esp8266</h1>
"capteurs_esp8266.ino" is responsible for reading data from sensors and then send that data to the esp32-cam using uart.<br>
Sensors are:<br>
max30100 (for heart beat and o2 level) using i2c / thermistor using adc / accelerometer gyroscope using i2c / oled lcd using i2c
<div align="center">
  <img src="./images/esp8266.jpg" alt="esp8266 image" width="200"/>
  <img src="./images/max30100.png" alt="max30100 image" width="200"/>
  <img src="./images/thermistor.webp" alt="thermistor image" width="200"/>
  <img src="./images/accelerometer.webp" alt="accelerometer image" width="200"/>
  <img src="./images/oled-i2c.jpg" alt="oled image" width="200"/>
</div>

<h1> Sensors data capture and transfer using an esp32-cam</h1>
"ble_esp32.ino" is responsible for receiving the data from the esp8266 using uart then initialing a ble server, service and characteristic. Then using a unique uuid for the service and the characteristic, advertise the data periodically as soon as it connects.
<div align="center">
  <img src="./images/esp32-cam.jpg" alt="esp32-cam image" width="200"/>
  <img src="./images/bluetooth_low_energy.png" alt="ble image" width="200"/>
</div>
<h1> Prediction model training and generation </h1>
"health.ipynb", we dicovered, prepared and used the data from "newborn_health_monitoring_with_risk.csv" found in Kaggle (https://www.kaggle.com/datasets/miadul/newborn-health-monitoring-dataset) to train a model.<br>
We generated this model into "my_health_classifier.joblib" file to be able to use the trained model and include it to the binary of the app we're building.<br>
We used "conv.py" to convert it to "health_classifier.onnx" so that we can include it to the app using an onnx runtime that we built for android found in : https://github.com/oussamaelwefi/onnx_runtime_for_android
We tested it using "test_onnx.py" <br>
We used a sickit-learn model in our case.<br>
<div align="center">
  <img src="./images/Scikit_learn_logo_small.svg.png" alt="sklearn logo" width="200"/>
  <img src="./images/Python-logo-notext.svg.png" alt="python logo" width="200"/>
  <img src="./images/joblib_logo.svg" alt="python logo" width="200"/>
  <img src="./images/Kaggle_Logo.svg.png" alt="python logo" width="200"/>
  <img src="./images/1200px-ONNX_logo_main.png" alt="python logo" width="200"/>
</div>
