
compiledb:
	pio run -t compiledb

tests:
	pio test --without-uploading -e native

build:
	pio run -e esp32dev 

run:
	pio run -e esp32dev --target upload

ota:
	pio run -e esp32dev-ota --target upload
