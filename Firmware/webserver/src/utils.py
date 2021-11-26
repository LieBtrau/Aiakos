# https://raw.githubusercontent.com/pfalcon/picoweb/b74428ebdde97ed1795338c13a3bdf05d71366a0/picoweb/utils.py

def unquote_plus(s):
    # TODO: optimize
    s = s.replace("+", " ")
    arr = s.split("%")
    arr2 = [chr(int(x[:2], 16)) + x[2:] for x in arr[1:]]
    return arr[0] + "".join(arr2)

def parse_qs(s):
    res = {}
    if s:
        pairs = s.split("&")
        for p in pairs:
            vals = [unquote_plus(x) for x in p.split("=", 1)]
            if len(vals) == 1:
                vals.append(True)
            old = res.get(vals[0])
            if old is not None:
                if not isinstance(old, list):
                    old = [old]
                    res[vals[0]] = old
                old.append(vals[1])
            else:
                res[vals[0]] = vals[1]
    return res

#print(parse_qs("foo"))
#print(parse_qs("fo%41o+bar=+++1"))
#print(parse_qs("foo=1&foo=2"))
