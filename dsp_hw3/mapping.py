# NOTICE: use 'big5-hkscs' as load/write file encoding

from argparse import ArgumentParser
import logging

def open_big52zhuyin_map(input_path):
    with open(input_path, "r", encoding="big5-hkscs") as f:
        big52zhuyin_map = f.read().split('\n')

    return big52zhuyin_map

def create_zhuyin2big5_dict(source_map):
    zhuyin2big5_dict = {}
    for line in source_map:
        # remove last empty line
        if len(line) < 3:
            continue
        big5, raw_zhuyin_lst = line.split(" ")
        raw_zhuyin_lst = raw_zhuyin_lst.split("/")
        # remove chinese tone
        zhuyin_lst = []
        for z in raw_zhuyin_lst:
            zhuyin_lst.append(z[0])
        # add big5 as one zhuyin dict key
        zhuyin_lst.append(big5)
        zhuyin_set = set(zhuyin_lst)
        for zhuyin in zhuyin_set:
            #zhuyin = raw_zhuyin[0]
            if zhuyin in zhuyin2big5_dict:
                zhuyin2big5_dict[zhuyin].append(big5)
            else:
                zhuyin2big5_dict[zhuyin] = [big5]
    return zhuyin2big5_dict

def save_zhuyin2big5_map(zhuyin2big5_dict, output_path):
    with open(output_path, "w", encoding="big5-hkscs") as f:
        for key, value in zhuyin2big5_dict.items():
            value_lst = ' '.join(value)
            line = key + "\t" + value_lst + '\n'
            f.write(line)
    

if __name__ == '__main__':
    parser = ArgumentParser(usage="ZhuYin-Big5 Parser", description="Turn Big5-ZhuYin map into ZhuYin-Big5 map file")
    parser.add_argument("FROM", help="Source Big5-ZhuYin map path")
    parser.add_argument("TO", help="Output ZhuYin-Big5 map")

    args = parser.parse_args()

    # make sure change logging level to "WARNING" after programming
    logging.basicConfig(level=logging.DEBUG) 

    logging.debug("From {}, To {}".format(args.FROM, args.TO))

    big52zhuyin_map = open_big52zhuyin_map(args.FROM)
    #print(big52zhuyin_map[:5])

    zhuyin2big5_dict = create_zhuyin2big5_dict(big52zhuyin_map)
    '''
    idx = 0
    for key, value in zhuyin2big5_dict.items():
        print(key, value)
        idx += 1
        if idx > 5:
            break
    '''
    save_zhuyin2big5_map(zhuyin2big5_dict, args.TO)

    logging.debug("Finish!")




