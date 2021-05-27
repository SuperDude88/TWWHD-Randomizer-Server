
import sys
import argparse
import hashlib
import subprocess
import os


def call_yaz0test(exe_loc, file_path, out_dir):
    fname = os.path.basename(file_path)
    dec_file_path = os.path.join(out_dir, fname + '.dec')
    result = subprocess.run([exe_loc, '-d', file_path, dec_file_path])
    if result.returncode != 0:
        print('Failed on decode step')
        return False, [dec_file_path]
    reencoded_file_path = dec_file_path + '.yaz0'
    result = subprocess.run([exe_loc, '-e', dec_file_path, reencoded_file_path])
    if result.returncode != 0:
        print('Failed on re-encode step')
        return False, [dec_file_path, reencoded_file_path]
    redecoded_file_path = reencoded_file_path + '.dec'
    result = subprocess.run([exe_loc, '-d', reencoded_file_path, redecoded_file_path])
    if result.returncode != 0:
        print('Failed on re-decode step')
        return False, [dec_file_path, reencoded_file_path, redecoded_file_path]

    with open(dec_file_path, 'rb') as first_stage:
        first_stage_digest = hashlib.sha256(first_stage.read()).hexdigest()
    
    with open(redecoded_file_path, 'rb') as second_stage:
        second_stage_digest = hashlib.sha256(second_stage.read()).hexdigest()
    
    if first_stage_digest != second_stage_digest:
        print('hash mismatch')
        return False, [dec_file_path, reencoded_file_path, redecoded_file_path]
    return True, [dec_file_path, reencoded_file_path, redecoded_file_path]
    

def clean_files(files):
    for file in files:
        try:
            os.remove(file)
        except OSError:
            print(f"couldn't delete file: {file}")


def main(args):
    parser = argparse.ArgumentParser()
    parser.add_argument('yaz0test', help='path to yaz0test')
    parser.add_argument('out_directory', help='path to generate resulting test files')
    parser.add_argument('--autoclean', type=bool, default=True)
    parser.add_argument('files', nargs='+', help='The yaz0 files to validate on')
    result = vars(parser.parse_args(args))
    out_dir = result['out_directory']
    autoclean = result['autoclean']
    file_list = result['files']
    if len(file_list) == 1 and os.path.isdir(file_list[0]):
        file_list = [os.path.join(file_list[0], f) for f in os.listdir(file_list[0])]
        file_list = list(filter(lambda f: os.path.isfile, file_list))
    os.makedirs(out_dir, exist_ok=True)
    for file in file_list:
        success, generated_files = call_yaz0test(result['yaz0test'], file, out_dir)
        if not success:
            print(f'failed on file {file}')
            return 1
        if autoclean:
            clean_files(generated_files)

    print('Success')
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
