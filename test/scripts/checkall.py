
import os
import sys
import hashlib
import argparse
import json
import subprocess

CKING_RPX_HASH = 'c4f0ab300542e0bfc462696850534e71db2ad02288a7eb55e5a4cd4062f16153'


def handle_rpx(test_exe_dir, rpx_path, work_dir, initial_hash, final_hash):
    test_exe = os.path.join(test_exe_dir, "rpxtest")
    if not os.path.isfile(test_exe):
        test_exe += '.exe'
        if not os.path.isfile(test_exe):
            return 'Unable to find rpxtest'
    output_file_path = rpx_path + '.elf'
    cmd = [test_exe, "-d", rpx_path, output_file_path]
    print(cmd)
    result = subprocess.run(cmd)
    if result.returncode != 0:
        return f'Unable to extract RPX file {rpx_path}'
    with open(output_file_path, 'rb') as output_file:
        output_hash = hashlib.sha256(output_file.read()).hexdigest()
    if output_hash != final_hash:
        return f'file {output_file_path} got a hash mismatch!'
    return None


def handle_yaz0(test_exe_dir, yaz0_path, work_dir, initial_hash, final_hash):
    expected_hash = final_hash
    if isinstance(final_hash, list):
        # TOOD: just require SARC file hash be first one for now
        expected_hash = final_hash[0]['hash']
    test_exe = os.path.join(test_exe_dir, 'yaz0test')
    if not os.path.isfile(test_exe):
        test_exe += '.exe'
        if not os.path.isfile(test_exe):
            return 'yaz0test executable not found!'
    if isinstance(final_hash, list):
        output_path = final_hash[0]['filename']
    else:
        output_path = os.path.basename(yaz0_path) + '.dec'
    output_path = os.path.join(work_dir, output_path)
    cmd = [test_exe, '-d', yaz0_path, output_path]
    result = subprocess.run(cmd)
    if result.returncode != 0:
        return f'Unable to extract Yaz0 file {yaz0_path}'
    with open(output_path, 'rb') as output_file:
        output_hash = hashlib.sha256(output_file.read()).hexdigest()
    if output_hash != expected_hash:
        return f'file {output_path} got a hash mismatch!'
    return None


def handle_sarc(test_exe_dir, sarc_path, work_dir, initial_hash, final_hash):
    if not isinstance(final_hash, list):
        return 'expected list for final hash for sarc type'
    final_hash_dict = {e['filename']: e['hash'] for e in final_hash}
    test_exe = os.path.join(test_exe_dir, 'sarctest')
    if not os.path.isfile(test_exe):
        test_exe += '.exe'
        if not os.path.isfile(test_exe):
            return 'sarctest executable not found!'
    cmd = [test_exe, "-u", work_dir, sarc_path]
    print(cmd)
    result = subprocess.run(cmd, capture_output=True)
    if result.returncode != 0:
        return 'Unable to unpack SARC'
    createdFiles = [f for f in result.stdout.decode().split('\n') if len(f) > 0]
    createdFiles = [f.replace('\r', '') for f in createdFiles]
    createdFiles = [os.path.join(work_dir, f) for f in createdFiles]
    for filepath in createdFiles:
        filename = os.path.basename(filepath)
        with open(filepath, 'rb') as testfile:
            filehash = hashlib.sha256(testfile.read()).hexdigest()
        if filename not in final_hash_dict:
            return f'File {filename} was produced but not expected'
        if filehash != final_hash_dict[filename]:
            return f'File {filename} has a non-matching hash'
    return None


def handle_entry(test_exe_dir, game_dir, work_dir, entry):
    file_path = entry['path']
    file_path = os.path.join(game_dir, file_path)
    entry_type = entry['type'].lower()
    entry_type_list = entry_type.split('@')
    initial_hash = entry['initialHash']
    final_hash = entry['finalHash']
    result = None
    for etype in entry_type_list:
        if etype.startswith('rpx'):
            result = handle_rpx(test_exe_dir, file_path, work_dir, initial_hash, final_hash)
            if result is not None:
                return result
        elif etype.startswith('yaz0'):
            result = handle_yaz0(test_exe_dir, file_path, work_dir, initial_hash, final_hash)
            if isinstance(final_hash, list):
                file_path = os.path.join(work_dir, final_hash[0]['filename'])
        elif etype.startswith('sarc'):
            result = handle_sarc(test_exe_dir, file_path, work_dir, initial_hash, final_hash)
        else:
            result = f'Unknown entry type {etype}'
        if result is not None:
            return result


def main(args):
    parser = argparse.ArgumentParser()
    parser.add_argument(
        'test_exe_dir',
        help='the directory containing test executables e.g. yaz0test'
    )
    parser.add_argument(
        "game_base_dir", 
        help="the dumped game's location; should contain code, content and meta"
    )
    parser.add_argument(
        'work_dir',
        help='The directory to keep temporary files in'
    )
    parser.add_argument(
        "hashes_file",
        default=os.path.join('data', 'hashes.json'),
        help="the specially formatted hashes file for testing"
    )
    result = vars(parser.parse_args(args))

    test_exe_dir = result['test_exe_dir']
    game_dir = result['game_base_dir']
    work_dir = result['work_dir']
    hash_file = result['hashes_file']
    if not os.path.isdir(game_dir):
        print('Cannot open game base directory')
        return 1
    cking_path = os.path.join(game_dir, 'code', 'cking.rpx')
    if not os.path.isfile(cking_path):
        print('Given game directory does not appear to be a complete dump')
        return 1
    with open(cking_path, 'rb') as cking_file:
        cking_hash = hashlib.sha256(cking_file.read()).hexdigest()
        if cking_hash != CKING_RPX_HASH:
            print("cking.rpx doesn't match what's expected; may be already modified")
            return 1

    with open(hash_file) as hash_file_handle:
        hash_data = json.load(hash_file_handle)

    os.makedirs(work_dir, exist_ok=True)

    for entry in hash_data:
        err = handle_entry(test_exe_dir, game_dir, work_dir, entry)
        if err is not None:
            print(err)
            return 1
        filepath = entry['path']
        print(f'Success for entry {filepath}')

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
