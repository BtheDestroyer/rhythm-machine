import os
import struct
import sys
import yaml

TOOL_VERSION = "1.0"
NOTE_VERSION_MAJOR = 1
NOTE_VERSION_MINOR = 0

def open_yaml(yaml_file):
    with open(yaml_file, "r") as stream:
        try:
            return yaml.safe_load(stream)
        except yaml.YAMLError as e:
            print("\tFailed to load YAML from file", yaml_file)
            print("\tException thrown:", e)
            return None

def print_usage(script_name):
    print("rhythm-machine song compiler version", TOOL_VERSION)
    print("https://github.com/BtheDestroyer/rhythm-machine/")
    print("Generates .note files for the rhythm-machine project from human-readable .yaml files.")
    print("\n\tUsage: python(3)", script_name, "file1.yaml [file2.yaml [...]]\n")

def validate_song_data(data):
    def validate_item(data, validator, field_name, print_error = True):
        if isinstance(validator, tuple):
            for option in validator:
                if validate_item(data, option, field_name, False):
                    return True
            if print_error:
                print("\tInvalid field:", field_name, "must be in the set", validator)
            return False
        if callable(validator):
            if not validator(data):
                if print_error:
                    print("\tInvalid field:", field_name)
                return False
        elif data is None:
            if print_error:
                print("\tMissing field:", field_name)
            return False
        elif isinstance(validator, dict):
            if not validate_with_dictionary(data, validator, field_name):
                if print_error:
                    print("\t\tInvalid child of field:", field_name)
                return False
        elif data != validator:
            if print_error:
                print("\tInvalid field:", field_name, "must be", validator)
            return False
        return True

    def validate_with_dictionary(data, dictionary, path = None):
        valid = True
        for key, validator in dictionary.items():
            field_name = path + "." + key if path != None else key
            this_item_valid = key in data and validate_item(data.get(key, None), validator, field_name)
            valid = valid and this_item_valid
        return valid
    
    def validate_array(array, dictionary, path):
        if not isinstance(array, list):
            print("\tInvalid field:", path, "must be an array")
            return False
        index = 0
        valid = True
        for item in array:
            field_name = path + "[" + str(index) + "]" if path != None else "[" + str(index) + "]"
            this_item_valid = validate_with_dictionary(item, dictionary, field_name)
            valid = valid and this_item_valid
            index += 1
        return True

    def positive_integer(x):
        return isinstance(x, int) and x >= 0

    ROOT_VALIDATION = {
        "song": {
            "ms_per_pixel": positive_integer,
            "lead_in_ms": positive_integer,
            "author": lambda x: isinstance(x, str) and len(x) <= 32,
            "difficulty": lambda x: isinstance(x, int) and x >= 1 and x <= 10
        },
        "notes": lambda x: validate_array(x, {
                "color": ("red", "green", "blue"),
                "direction": ("left", "right"),
                "start_ms": positive_integer,
                "length_ms": positive_integer,
                "speed": lambda x: x is None or ((isinstance(x, int) or isinstance(x, float)) and x > 0),
            }, "notes")
    }
    return validate_with_dictionary(data, ROOT_VALIDATION)

def main(argv):
    if len(argv) < 2:
        print_usage(argv[0])
        return

    yaml_files = argv[1:]
    dry_run = False
    if yaml_files[0] in ["--dry", "-d"]:
        yaml_files = yaml_files[1:]
        dry_run = True
    
    for yaml_file in yaml_files:
        print("Reading:", yaml_file)
        data = open_yaml(yaml_file)
        if data == None or not validate_song_data(data):
            print("\t\tInvalid YAML data; skipping")
            continue
        if dry_run:
            continue
        print("\tCompiling...")
        note_file = open(os.path.splitext(yaml_file)[0] + ".note", "wb")
        note_file.write(struct.pack("4c", *[bytes(x, 'utf-8') for x in "NOTE"]))
        note_file.write(struct.pack("<H", NOTE_VERSION_MAJOR))
        note_file.write(struct.pack("<H", NOTE_VERSION_MINOR))
        note_file.write(struct.pack("<I", data["song"]["ms_per_pixel"]))
        note_file.write(struct.pack("<I", len(data["notes"])))
        author_fixed = (list(data["song"].get("author", "")) + ['\0'] * 32)[:32]
        note_file.write(struct.pack("32c", *[bytes(c, 'utf-8') for c in author_fixed]))
        note_file.write(struct.pack("<b", data["song"]["difficulty"]))
        # padding for future header data
        note_file.write(struct.pack("31c", *[bytes(c, 'utf-8') for c in ['\0'] * 31]))
        for note in data["notes"]:
            note_file.write(struct.pack("<b", ["red", "green", "blue"].index(note["color"])))
            note_file.write(struct.pack("<b", ["left", "right"].index(note["direction"])))
            note_file.write(struct.pack("<I", note["start_ms"] + data["song"]["lead_in_ms"]))
            note_file.write(struct.pack("<I", note["length_ms"]))
            note_file.write(struct.pack("<f", note.get("speed", 1.0)))
            # padding for future note data
            note_file.write(struct.pack("2c", *[bytes(c, 'utf-8') for c in ['\0'] * 2]))
        print("\tDone!")
    
if __name__ == "__main__":
    main(sys.argv)
