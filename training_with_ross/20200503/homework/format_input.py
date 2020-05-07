import re

_args_pattern = re.compile(r'\[(.*?)\]')
_param_idx = 0


def printCtor(capacity):
    print('int capacity = {};'.format(capacity));
    print('LFUCache* obj = new LFUCache(capacity);')
    print()


def printGet(key):
    global _param_idx
    _param_idx += 1
    print('int param_{} = obj->get({});'.format(_param_idx, key))
    print('cout << "int param_{} = obj->get({}); " << param_{} << endl;'.format(_param_idx, key, _param_idx))
    print()


def printPut(key, val):
    print('cout << "obj->put({}, {});" << endl;'.format(key, val))
    print('obj->put({}, {});'.format(key, val))
    print()


def main():
    op_string = input('Type the two input rows:\n')
    arg_string = input()
    
    op_string = op_string.strip()[1: -1]
    arg_string = arg_string.strip()[1: -1]
    
    ops = map(lambda s: s[1: -1], op_string.split(','))
    args = _args_pattern.findall(arg_string)
    args = map(lambda s: int(s) if len(s) == 1 else (int(s[0]), int(s[2])), args)
    
    pairs = list(zip(ops, args))
    printCtor(pairs[0][1])
    for op, arg in pairs[1:]:
        if op == 'get':
            printGet(arg)

        elif op == 'put':
            printPut(*arg)


if __name__ == '__main__':
    main()

