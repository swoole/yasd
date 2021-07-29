# Tests

Make sure you have compiled and install `yasd`, then

```bash
cd tests
```

## Create test

```bash
# eg: ./new.php yasd_debugger/remote_mode
./new.php path/to/directory
```

## Run test

## Run all tests

```bash
PHP_INI_SCAN_DIR=$PWD/extension_ini ./start.sh
```

## Run specific test

```bash
# eg: PHP_INI_SCAN_DIR=$PWD/extension_ini ./start.sh yasd_debugger/remote_mode/phpstorm
PHP_INI_SCAN_DIR=$PWD/extension_ini ./start.sh path/to/directory

# eg: PHP_INI_SCAN_DIR=$PWD/extension_ini ./start.sh yasd_debugger/remote_mode/phpstorm/compatible_with_phpstorm.phpt
PHP_INI_SCAN_DIR=$PWD/extension_ini ./start.sh path/to/file
```
