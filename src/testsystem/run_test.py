import sys, os
import xml.etree.ElementTree as ET
import shutil, subprocess, filecmp

assert sys.version_info >= (3,1)
assert len(sys.argv) >= 2

include_dir0 = "F:\\Microsoft visual Studio 2014\\VC\\include"
include_dir1 = "C:\\Program Files (x86)\\Windows Kits\\10\\Include\\10.0.14393.0\\ucrt"

def find_compiler(root):
	assert os.path.isdir(root)
	for subdir, dirs, files in os.walk(root):
		for file in files:
			if file == 'tyx.exe':
				filepath = os.path.join(subdir, file)
				print("[OK] Found compiler at " + filepath)
				return filepath
	return ''

def run_test_from_file(test_file):
	print("[OK] Running test file: " + str(test_file))
	tree = ET.parse(test_file)
	root = tree.getroot()
	if root.tag != 'tytest':
		print("[ERROR] Missing 'tytest' attribute. Root is '" + root.tag + "'. Not a valid .tytest file")
		return 1
	sample = {}
	expected = {}
	checker = {}
	for child in root:
		if child.tag == 'sample':
			sample = child.text
		if child.tag == 'expected':
			expected = child.text
		if child.tag == 'checker':
			checker = child.text
	assert sample
	assert expected
	assert checker

	if not os.path.isdir('tmp'):
		os.mkdir('tmp')
	with open('tmp/sample.ty', 'w') as text_file:
		text_file.write(sample)
	with open('tmp/expected.c', 'w') as text_file:
		text_file.write(expected)
	with open('tmp/checker.c', 'w') as text_file:
		text_file.write(checker)
	print("[OK] Created temporary source files")

	compilerpath = find_compiler("..")
	if compilerpath == '':
		compilerpath = find_compiler("../..")
	assert compilerpath != ''

	print("[OK] Compiling checker..")
	subprocess.call(['clang++', '-emit-llvm', '-I', include_dir0, '-I', include_dir1, '-S', 'tmp/checker.c', '-o', 'tmp/checker.s'])
	subprocess.call(['llvm-as', 'tmp/checker.s', '-o', 'tmp/checker.bc'])

	print("[OK] Compiling expected..")
	subprocess.call(['clang++', '-emit-llvm', '-I', include_dir0, '-I', include_dir1, '-S', 'tmp/expected.c', '-o', 'tmp/expected.s'])
	subprocess.call(['llvm-as', 'tmp/expected.s', '-o', 'tmp/expected.bc'])

	print("[OK] Executing " + compilerpath + " tmp/sample.ty > sample.s")
	with open('tmp/sample.s', 'w') as outfile:
		subprocess.call([compilerpath, 'tmp/sample.ty'], stdout=outfile)
	subprocess.call(['llvm-as', 'tmp/sample.s', '-o', 'tmp/sample.bc'])

	subprocess.call(['llvm-link', 'tmp/sample.bc', 'tmp/checker.bc', '-o', 'tmp/actual.bc'])
	subprocess.call(['llvm-link', 'tmp/expected.bc', 'tmp/checker.bc', '-o', 'tmp/expected.bc'])

	with open('tmp/expected.out', 'w') as outfile:
		subprocess.call(['lli', 'tmp/expected.bc'], stdout=outfile)

	with open('tmp/actual.out', 'w') as outfile:
		subprocess.call(['lli', 'tmp/actual.bc'], stdout=outfile)

	if filecmp.cmp('tmp/actual.out', 'tmp/expected.out'):
		print('[PASS] ' + test_file)
		shutil.rmtree('tmp')
		print("[OK] Cleared temporary source files")
	else:	
		print('[FAIL] ' + test_file)
		subprocess.call(['diff', 'tmp/actual.out', 'tmp/expected.out'])
		# subprocess.call(['diff', 'tmp/sample.s', 'tmp/expected.s'])

	return 0

def run_test_from_dir(test_dir):
	print("[OK] Emumerating test directory: " + str(input))
	for subdir, dirs, files in os.walk(test_dir):
		for file in files:
			filepath = os.path.join(subdir, file)
			if filepath.endswith('.tytest'):
				run_test_from_file(filepath)
	return 0

if __name__ == '__main__':
	input = sys.argv[1]

	if os.path.isfile(input):
		run_test_from_file(input)
	elif os.path.isdir(input):
		run_test_from_dir(input)
	else:
		print("[ERROR] Invalid argument (not a file or directory): " + str(input))
		exit(1)
