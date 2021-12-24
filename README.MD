# TP-Link SmartPlug Metric Extractor

This is a small tool to extract metric from TP-Links Kasa devices. I was havong some trouble using the Python script from <https://github.com/softScheck/tplink-smartplug>. It was using a lot of CPU time. Since I dont know how to write Python, I had to wirte it n something different. My choise was C.

It should work with most of there SmartPlugs. I'm using it on a KP115. This is based on the code by softScheck. You find the orignal repo at <https://github.com/softScheck/tplink-smartplug>.

## Build

You can build it in just a few seconds. Make sure you have the latest version of `gcc` (Linux; debian: `apt instal gcc`) or `clang` (macOS via Xcode) installed.

1. Download using git OR using wget

    - using git (debian: `apt install git`)

        ```bash
        git clone https://github.com/janwiesemann/      TPLinkSmartPlugMetricExtractor.git
        cd TPLinkSmartPlugMetricExtractor
        ````

    - using wget (debina: `apt install wget`)

        ```bash
        mkdir TPLinkSmartPlugMetricExtractor
        cd TPLinkSmartPlugMetricExtractor
        wget https://raw.githubusercontent.com/janwiesemann/    TPLinkSmartPlugMetricExtractor/master/main.cpp
        ```

2. compiling

    ```bash
    gcc -g main.cpp -o TPLinkSmartPlugMetricExtractor
    ```

3. make it executable

    ```bash
    chmod +x TPLinkSmartPlugMetricExtractor
    ````

## Execute it

You have to specify the devices IP using the first Argument.

```bash
./TPLinkSmartPlugMetricExtractor 10.1.2.3
```

## Using it in Telegraf

Telegraf can be used to insert the values into a time series database like InfluxDB or Prometheus.

Add this to your `/etc/telegraf/telegraf.conf`or if you want a clean solution you can use `/etc/telegraf/telegraf.d/TPSmartPlugs.conf`

```ini
[[inputs.exec]]
  commands = ["{PATH TO EXECUTABLE}/TPLinkSmartPlugMetricExtractor 10.1.2.3"]
  timeout = "1s"
  data_format = "json"
  [inputs.exec.tags]
    host="{HOSTNAME}"
```

Note: `[inputs.exec.tags]` is used to override the hostname for better filtering. You can remove this and the following values if you dont want to use this tag.
