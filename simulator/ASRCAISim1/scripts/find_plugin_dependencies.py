# Copyright (c) 2024-2025 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)

def get_plugin_directory(name):
    import os
    import importlib
    m = importlib.import_module(name)
    return os.path.abspath(os.path.dirname(m.__file__)).replace(os.path.sep,'/')

if __name__ == '__main__':
    import json
    import sys
    pluginDependencyInfo = json.load(open(sys.argv[1],"r"))
    if len(pluginDependencyInfo) > 0:
        print(';'.join([get_plugin_directory(p) for p, _ in pluginDependencyInfo.items()]), end='')
    else:
        print('')
