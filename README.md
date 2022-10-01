# Wireless and Mobile Networks
## Convolutional encoder

## Build
Use `make` command to build this project.

## Run
Tool can be run with 3 different options.

### Help
```bash
./bms -h
```
Shows help how to use this tool with its quick description.

### Encode
```bash
./bms -e [-b x y z] <<< message
```
Encodes message. Optionaly, 3 numbers can be used to specify encoder details, where:
x = Number of delays blocks
y = Upper feedback
z = Lower feedback
By default, this numbers are x = 5, y = 53 and z = 46.

### Decode
```bash
./bms -d [-b x y z] <<< message
```
Decodes message. Optionaly, 3 numbers can be used to specify encoder details, where:
x = Number of delays blocks
y = Upper feedback
z = Lower feedback
By default, these numbers are x = 5, y = 53 and z = 46.