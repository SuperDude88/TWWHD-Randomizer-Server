
import sys
import os
import argparse
import hashlib
import subprocess


def cleanFiles(files):
    for f in files:
        try:
            os.remove(f)
        except OSError:
            pass


def validateOnSARC(executable_path, sarc_path, work_dir, do_delete = True):
    os.makedirs(work_dir, exist_ok=True)
    cmd = [executable_path, "-u", work_dir, sarc_path]
    print(cmd)
    result = subprocess.run(cmd, capture_output=True)
    if result.returncode != 0:
        print('Unable to unpack SARC')
        return False
    createdFiles = [f for f in result.stdout.decode().split('\n') if len(f) > 0]
    createdFiles = [os.path.join(work_dir, f) for f in createdFiles]
    sarcName = os.path.basename(sarc_path)
    outputSarc = os.path.join(work_dir, sarcName + '.check')
    result = subprocess.run([executable_path, '-p', outputSarc] + createdFiles)
    createdFiles.append(outputSarc)
    if result.returncode != 0:
        print('unable to repack SARC')
        if do_delete: cleanFiles(createdFiles)
        return False

    with open(sarc_path, 'rb') as original:
        original_hash = hashlib.sha256(original.read()).hexdigest()
    with open(outputSarc, 'rb') as repack:
        repack_hash = hashlib.sha256(repack.read()).hexdigest()

    if original_hash != repack_hash:
        print('Hash mismatch!')
        if do_delete: cleanFiles(createdFiles)
        return False

    print('success!')
    if do_delete: cleanFiles(createdFiles)
    return True


def main(args):
    parser = argparse.ArgumentParser()
    parser.add_argument('executable', help='path to the sarctest binary')
    parser.add_argument(
        'sarcFile', 
        help='the sarc file to test with, or a dir containing sarc files to test on'
    )
    parser.add_argument('workdir', default='.')
    parser.add_argument(
        '--filter-ext', 
        default='.sarc', 
        help='if dir is given for sarcFile, a filter to select files based on extension'
    )
    parser.add_argument(
        '--no-clean', 
        action='store_true',
        help='if provided, created temp files will not be deleted'
    )
    result = vars(parser.parse_args(args))

    work_dir = result['workdir']
    sarc_file = result['sarcFile']
    ext = result['filter_ext']
    if os.path.isdir(sarc_file):
        file_list = os.listdir(sarc_file)
        file_list = list(filter(lambda f: os.path.splitext(f)[1] == ext, file_list))
        outlist = [os.path.join(work_dir, os.path.splitext(f)[0]) for f in file_list]
        file_list = [os.path.join(sarc_file, f) for f in file_list]
    else:
        file_list = [sarc_file]
        outlist = [work_dir]

    if len(file_list) == 0:
        print('No usable sarc files found!')
        return 1

    print('Will process {}'.format([os.path.basename(f) for f in file_list]))

    for out, filepath in zip(outlist, file_list):
        fname = os.path.basename(filepath)
        print(f'Checking on {fname}')
        success = validateOnSARC(
            result['executable'], 
            filepath,
            out,
            not result['no_clean']
        )
        if not success:
            print(f'failed on file: {fname}')
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))

