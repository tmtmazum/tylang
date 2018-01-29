import sys, os
import xml.etree.ElementTree as ET
import shutil

assert sys.version_info >= (3,1)
assert len(sys.argv) >= 2

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

	shutil.rmtree('tmp')
	print("[OK] Cleared temporary source files")
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
