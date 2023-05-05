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
    valid = True
    if "song" not in data:
        print("\tMissing field: song")
        valid = False
    elif not isinstance(data["song"], dict):
        print("\tInvalid field: song must be a dictionary")
        valid = False
    if "ms_per_pixel" not in data.get("song", {}):
        print("\tMissing field: song.ms_per_pixel")
        valid = False
    elif not isinstance(data["song"]["ms_per_pixel"], int) or data["song"]["ms_per_pixel"] < 0:
        print("\tInvalid field: song.ms_per_pixel must be a positive integer")
        valid = False
    if "lead_in_ms" not in data.get("song", {}):
        print("\tMissing field: song.lead_in_ms")
        valid = False
    elif not isinstance(data["song"]["lead_in_ms"], int) or data["song"]["lead_in_ms"] < 0:
        print("\tInvalid field: song.lead_in_ms must be a positive integer")
        valid = False
    if "author" in data.get("song", {}):
        if not isinstance(data["song"]["author"], str) or len(data["song"]["author"]) > 32:
            print("\tInvalid field: song.author must be a string of length less than or equal to 32")
            valid = False
    if "difficulty" not in data.get("song", {}):
        print("\tMissing field: song.lead_in_ms")
        valid = False
    if "difficulty" not in data.get("song", {}):
        print("\tMissing field: song.difficulty")
        valid = False
    elif not isinstance(data["song"]["difficulty"], int) or data["song"]["difficulty"] < 1 or data["song"]["difficulty"] > 10:
        print("\tInvalid field: song.difficulty must be an integer between 1 and 10 (inclusive)")
        valid = False

    if "notes" not in data:
        print("\tMissing field: notes")
        valid = False
    elif not isinstance(data["notes"], list):
        print("\tInvalid field: notes must be a list")
        valid = False
    else:
        note_index = 0
        for note in data["notes"]:
            if "color" not in note:
                print("\tMissing field: note[", note_index, "].color", sep="")
                valid = False
            elif note["color"] not in ["red", "green", "blue"]:
                print("\tInvalid field: note[", note_index, "].color must be of the set: [red, green, blue]", sep="")
                valid = False
            if "direction" not in note:
                print("\tMissing field: note[", note_index, "].direction", sep="")
                valid = False
            elif note["direction"] not in ["left", "right"]:
                print("\tInvalid field: note[", note_index, "].direction must be of the set: [left, right]", sep="")
                valid = False
            if "start_ms" not in note:
                print("\tMissing field: note[", note_index, "].start_ms", sep="")
                valid = False
            elif not isinstance(note["start_ms"], int) or note["start_ms"] < 0:
                print("\tInvalid field: note[", note_index, "].start_ms must be a positive integer (value: ", note["start_ms"], ")", sep="")
                valid = False
            if "length_ms" not in note:
                print("\tMissing field: note[", note_index, "].length_ms", sep="")
                valid = False
            elif not isinstance(note["length_ms"], int) or note["length_ms"] < 0:
                print("\tInvalid field: note[", note_index, "].length_ms must be a positive integer (value: ", note["length_ms"], ")", sep="")
                valid = False
            if "speed" in note:
                if (not isinstance(note["speed"], int) and not isinstance(note["speed"], float)) or note["speed"] <= 0:
                    print("\tInvalid field: note[", note_index, "].speed must be a non-zero positive number (value: ", note["speed"], ")", sep="")
                    valid = False
            note_index += 1
    return valid

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
        print("\tDone!")
    
if __name__ == "__main__":
    main(sys.argv)
