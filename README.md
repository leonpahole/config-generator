# Configuration generator

This utility can generate configuration files on basis of conditionals and variable 
substitution of environment file against a template file.

### Install

```
mkdir build
cd build
cmake ..
make
sudo make install
```

### Examples

configuration.template - the template file for configuration
```
server {

    listen %{PORT};
    server_name %{SERVER_NAME};

    %IF %{ENVIRONMENT} IS PRODUCTION
    ssl_certificate %{CERT_PATH}
    ssl_certificate_key %{CERT_KEY_PATH}
    ssl_trusted_certificate %{TRUSTED_CERT_PATH}
    %ENDIF
}
```

configuration.env - the environment file for configuration
```
PORT=3000
SERVER_NAME=example.com
CERT_PATH=/home/example/cert
CERT_KEY_PATH=/home/example/cert_key
TRUSTED_CERT_PATH=/home/example/trusted_cert
ENVIRONMENT=LOCAL
```

Running 
```
config-generator --env configuration.env --file configuration.template --out configuration.conf
```

gives us configuration.conf as follows

```
server {

    listen 3000;
    server_name example.com;
}
```

### Command-line arguments

You can run ``config-generator`` with following parameters:

* ``--env``: path to environment file. You can specify more files by adding multiple ``-env`` flags. 
If variables in files overlap, warnings will be issues, but the variable in latter file will take precedence.

* ``--file``: path to configuration file. You can specify more files by adding multiple ``-file`` flags.

* ``--dir``: substitute all files in a directory.

* ``--out``: name of output file or directory, depending on whether you used ``--file`` or ``-dir``. 
If you specified multiple files, you need to specify multiple outputs as well. 
The inputs will be mapped to outputs based on the sequence.

* ``--stdout``: instead of writing to file, output the result to stdout. 
Can be used with ``-out`` to combine writing to files and priting to stdout.

Notice: ``--dir`` and ``--file`` can not be used at the same time (for now). 
You can also specify only one ``--dir`` at once.

You can also set some configuration settings.

* ``--definer``: change the default value for a prefix from ``%`` to any character you like, such as ``#``. 
The only characters that might not work here are regex characters, like ``$`` or ``^``.

* ``--case-sensitive``: by default the comparisons are case insensitive. You can make it case sensitive by adding this flag.

Examples:

```
# one env and one template file -> one output file
config-generator --env configuration.env --file configuration.template --out configuration.conf

# one env and multiple template files -> multiple output files
config-generator --env configuration.env --file configuration.template --file configuration2.template --out configuration.conf --out configuration2.conf

# one env and one directory -> one output directory
config-generator --env configuration.env --dir configuration-directory.template ---out configuration-directory

# multiple envs
config-generator --env configuration.env --env configuration2.env --file configuration.template --out configuration.conf

# printing to stdout instead of saving the files
config-generator --env configuration.env --file configuration.template --stdout
```

### Language specification

#### Replacement

To replace variables, use ``%{VARIABLE_NAME}``. 
Please note that this will replace every pattern in text, even if the pattern is not separated with white space 
(eg. ``hello%{VARIABLE_NAME}world``).

#### Conditionals

For conditionals, use ``%IF CONDITION``. 
The statement needs to end with ``%ENDIF`` to define a block that will be conditionally displayed or not.

Conditions can include multiple conditionals, separated with logical operators ``AND`` and ``OR``.

Condition has the structure ``VALUE_OR_VARIABLE COMPARATOR VALUE_OR_VARIABLE``. Variables need to be preceded by ``%``.

Comparators are comparisons operators:

* ``IS`` for equality
* ``IS_NOT`` for inequality

Examples of conditionals:

* ``%IF %{PORT} IS 3000``
* ``%IF %{SSL_ON} IS_NOT true``
* ``%IF false IS_NOT %{SSL_ON}``
* ``%IF %{LOCALE} IS %{MY_LOCALE}`` (comparing two variables)

Note: variables in IF statements will be replaced literally. 
This means that for example ``%IF aaa%{LOCALE} IS PRODUCTION`` with ``LOCALE=production`` will be replaced with 
``IF aaaproduction IS PRODUCTION``, which will be evaluated as ``false``.
