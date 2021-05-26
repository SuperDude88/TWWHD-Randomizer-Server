
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


def validateOnSARC(executablePath, sarcPath, workDir):
    os.makedirs(workDir, exist_ok=True)
    result = subprocess.run([executablePath, "-u", workDir, sarcPath])
    if result.returncode != 0:
        print('Unable to unpack SARC')
        return 1

    createdFiles = os.listdir(workDir)
    createdFiles = [os.path.join(workDir, f) for f in createdFiles]
    sarcName = os.path.basename(sarcPath)
    outputSarc = os.path.join(workDir, sarcName + '.check')
    result = subprocess.run([executablePath, '-p', outputSarc] + createdFiles)
    createdFiles.append(outputSarc)
    if result.returncode != 0:
        print('unable to repack SARC')
        cleanFiles(createdFiles)
        return 1

    with open(sarcPath, 'rb') as original:
        original_hash = hashlib.sha256(original.read()).hexdigest()
    with open(outputSarc, 'rb') as repack:
        repack_hash = hashlib.sha256(repack.read()).hexdigest()

    if original_hash != repack_hash:
        print('Hash mismatch!')
        cleanFiles(createdFiles)
        return 1

    print('success!')
    cleanFiles(createdFiles)
    return 0


def main(args):
    parser = argparse.ArgumentParser()
    parser.add_argument('executable', help='path to the sarctest binary')
    parser.add_argument('sarcFile', help='the sarc file to test with')
    parser.add_argument('workdir', default='.')
    result = vars(parser.parse_args(args))
    return validateOnSARC(result['executable'], result['sarcFile'], result['workdir'])


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))

