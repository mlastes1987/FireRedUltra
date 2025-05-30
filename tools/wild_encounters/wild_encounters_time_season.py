import json
import re

mon_types = ["land_mons", "water_mons", "rock_smash_mons", "fishing_mons", "hidden_mons"]

seasons = {
    "SEASON_SPRING": "Spring",
    "SEASON_SUMMER": "Summer",
    "SEASON_AUTUMN": "Autumn",
    "SEASON_WINTER": "Winter"
}

times_of_day = {
    "TIME_MORNING": "Morning",
    "TIME_DAY": "Day",
    "TIME_EVENING": "Evening",
    "TIME_NIGHT": "Night"
}

base_season = "SEASON_SPRING"
base_time = "TIME_MORNING"

class Config:
    def __init__(self, config_file_name):
        self.time_encounters = None
        self.disable_time_fallback = None
        self.time_fallback = None
        self.season_encounters = None
        self.disable_season_fallback = None
        self.season_fallback = None

        with open(config_file_name, 'r') as config_file:
            lines = config_file.readlines()
            for line in lines:
                self.ParseSeasonConfig(line)
                self.ParseTimeConfig(line)
        
        if self.time_encounters == None:
            raise Exception("OW_TIME_OF_DAY_ENCOUNTERS not defined.")
        if self.disable_time_fallback == None:
            raise Exception("OW_TIME_OF_DAY_DISABLE_FALLBACK not defined.")
        if self.time_fallback == None:
            raise Exception("OW_TIME_OF_DAY_FALLBACK not defined.")
        if self.season_encounters == None:
            raise Exception("OW_SEASON_ENCOUNTERS not defined.")
        if self.disable_season_fallback == None:
            raise Exception("OW_SEASON_DISABLE_FALLBACK not defined.")
        if self.season_fallback == None:
            raise Exception("OW_SEASON_FALLBACK not defined.")

    def ParseSeasonConfig(self, line):
        m = re.search(r'#define OW_SEASON_ENCOUNTERS\s+(\w+)', line)
        if m:
            self.season_encounters = m.group(1) == "TRUE"

        m = re.search(r'#define OW_SEASON_DISABLE_FALLBACK\s+(\w+)', line)
        if m:
            self.disable_season_fallback = m.group(1) == "TRUE"

        m = re.search(r'#define OW_SEASON_FALLBACK\s+(\w+)', line)
        if m:
            self.season_fallback = m.group(1)

    def ParseTimeConfig(self, line):
        m = re.search(r'#define OW_TIME_OF_DAY_ENCOUNTERS\s+(\w+)', line)
        if m:
            self.time_encounters = m.group(1) == "TRUE"

        m = re.search(r'#define OW_TIME_OF_DAY_DISABLE_FALLBACK\s+(\w+)', line)
        if m:
            self.disable_time_fallback = m.group(1) == "TRUE"

        m = re.search(r'#define OW_TIME_OF_DAY_FALLBACK\s+(\w+)', line)
        if m:
            self.time_fallback = m.group(1)

class WildEncounterAssembler:
    def __init__(self, output_file, json_data, config):
        self.output_file = output_file
        self.json_data = json_data
        self.config = config
    
    def WriteLine(self, line="", indents = 0):
        self.output_file.write(4 * indents * " " + line + "\n")

    def WriteHeader(self):
        self.WriteLine("//")
        self.WriteLine("// DO NOT MODIFY THIS FILE! It is auto-generated by tools/wild_encounters/wild_encounters_time_season.py")
        self.WriteLine("//")
        self.output_file.write("\n\n")

    def WriteMacro(self, macro, value):
        self.output_file.write("#define " + macro + " " + value + "\n")

    def WriteMacros(self):
        wild_encounter_groups = self.json_data["wild_encounter_groups"]
        for wild_encounter_group in wild_encounter_groups:
            if "fields" in wild_encounter_group:
                fields = wild_encounter_group["fields"]
                for field in fields:
                    field_type = field["type"]
                    macro_base = "ENCOUNTER_CHANCE_" + field_type.upper()
                    previous_group = None
                    previous_macro = None
                    encounter_rates = field["encounter_rates"]

                    group_name_mapping = len(encounter_rates) * [""]
                    if "groups" in field:
                        groups = field["groups"]
                        for group_name, indices in groups.items():
                            for index in indices:
                                group_name_mapping[index] = "_" + group_name.upper()
                    
                    for idx, rate in enumerate(encounter_rates):
                        macro_name = macro_base + group_name_mapping[idx] + "_SLOT_" + str(idx)
                        macro_value = str(rate)
                        if previous_group == group_name_mapping[idx]:
                            macro_value = "(" + previous_macro + " + " + macro_value + ")"
                        elif idx > 0:
                            macro_total_name = macro_base + group_name_mapping[idx - 1] + "_TOTAL"
                            self.WriteMacro(macro_total_name, "(" + previous_macro + ")")
                        self.WriteMacro(macro_name, macro_value)
                        previous_group = group_name_mapping[idx]
                        previous_macro = macro_name
                        if idx == len(encounter_rates) - 1:
                            macro_total_name = macro_base + group_name_mapping[idx] + "_TOTAL"
                            self.WriteMacro(macro_total_name, "(" + previous_macro + ")")
                    macro_total_name = macro_base + group_name_mapping[-1] + "_TOTAL"
                    self.WriteLine()
    
    def WriteMonInfos(self, name, mons, encounter_rate):
        info_name = name + "Info"
        self.WriteLine(f"const struct WildPokemon {name}[] =")
        self.WriteLine("{")
        for mon in mons:
            species = mon["species"]
            min_level = 2 if "min_level" not in mon else mon["min_level"]
            max_level = 100 if "max_level" not in mon else mon["max_level"]
            self.WriteLine(f"{{ {min_level}, {max_level}, {species} }},", 1)

        self.WriteLine("};")
        self.WriteLine()
        self.WriteLine(f"const struct WildPokemonInfo {info_name} = {{ {encounter_rate}, {name} }};")
        self.WriteLine()
    
    def WriteTerminator(self):
        self.WriteLine("{", 1)
        self.WriteLine(".mapGroup = MAP_GROUP(MAP_UNDEFINED),", 2)
        self.WriteLine(".mapNum = MAP_NUM(MAP_UNDEFINED),", 2)
        self.WriteLine(".encounterTypes =", 2)
        self.WriteLine("{", 2)
        for season in seasons:
            if not self.config.season_encounters and season != self.config.season_fallback:
                continue
            self.WriteLine(f"[{season}] =", 3)
            self.WriteLine("{", 3)
            for time in times_of_day:
                if not self.config.time_encounters and time != self.config.time_fallback:
                    continue
                self.WriteLine(f"[{time}] =", 4)
                self.WriteLine("{", 4)
                for mon_type in mon_types:
                    member_name = mon_type.title().replace("_", "")
                    member_name = member_name[0].lower() + member_name[1:] + "Info"
                    self.WriteLine(f".{member_name} = NULL,", 5)
                self.WriteLine("},", 4)
            self.WriteLine("},", 3)
        self.WriteLine("},", 2)
        self.WriteLine("},", 1)

    def WritePokemonHeaders(self, headers):
        label = headers["label"]
        self.WriteLine(f"const struct WildPokemonHeader {label}[] =")
        self.WriteLine("{")
        for shared_label in headers["data"]:
            self.WriteLine()
            map_data = headers["data"][shared_label]
            encounter_data = map_data
            map_group = map_data["mapGroup"]
            map_num = map_data["mapNum"]
            version = "FIRERED"
            if "LeafGreen" in shared_label:
                version = "LEAFGREEN"
            
            self.WriteLine(f"#ifdef {version}")

            self.WriteLine("{", 1)
            self.WriteLine(f".mapGroup = {map_group},", 2)
            self.WriteLine(f".mapNum = {map_num},", 2)
            self.WriteLine(".encounterTypes =", 2)
            self.WriteLine("{", 2)
            for season in seasons:
                if not self.config.season_encounters and season != self.config.season_fallback:
                    continue
                self.WriteLine(f"[{season}] =", 3)
                self.WriteLine("{", 3)
                for time in times_of_day:
                    if not self.config.time_encounters and time != self.config.time_fallback:
                        continue
                    self.WriteLine(f"[{time}] =", 4)
                    self.WriteLine("{", 4)
                    for mon_type in mon_types:
                        member_name = mon_type.title().replace("_", "")
                        member_name = member_name[0].lower() + member_name[1:] + "Info"
                        value = "NULL"
                        if season in encounter_data and time in encounter_data[season] and mon_type in encounter_data[season][time]:
                            value = encounter_data[season][time][mon_type]
                        if value != "NULL":
                            value = "&" + value
                        self.WriteLine(f".{member_name} = {value},", 5)

                    self.WriteLine("},", 4)
                self.WriteLine("},", 3)
            
            self.WriteLine("},", 2)
            self.WriteLine("},", 1)
            self.WriteLine(f"#endif")
        self.WriteTerminator()
        self.WriteLine("};")

                
    def WriteEncounters(self):
        wild_encounter_groups = self.json_data["wild_encounter_groups"]
        for wild_encounter_group in wild_encounter_groups:
            headers = {}
            headers["label"] = wild_encounter_group["label"]
            headers["data"] = {}
            for_maps = False
            map_num_counter = 1
            if "for_maps" in wild_encounter_group:
                for_maps = wild_encounter_group["for_maps"]
            encounters = wild_encounter_group["encounters"]

            for map_encounters in encounters:
                map_name = map_encounters["map"]
                map_group = "0"
                map_num = str(map_num_counter)
                map_num_counter += 1
                if for_maps:
                    map_group = f"MAP_GROUP({map_name})"
                    map_num = f"MAP_NUM({map_name})"
                base_label = map_encounters["base_label"]
                shared_label = base_label
                season = self.config.season_fallback
                time = self.config.time_fallback

                for season_ident in seasons:
                    if seasons[season_ident] in base_label:
                        season = season_ident
                        shared_label = shared_label.replace('_' + seasons[season_ident], '')
                for time_ident in times_of_day:
                    if times_of_day[time_ident] in base_label:
                        time = time_ident
                        shared_label = shared_label.replace('_' + times_of_day[time_ident], '')

                if shared_label not in headers["data"]:
                    headers["data"][shared_label] = {}
                if season not in headers["data"][shared_label]:
                    headers["data"][shared_label][season] = {}
                if time not in headers["data"][shared_label][season]:
                    headers["data"][shared_label][season][time] = {}
                headers["data"][shared_label]["mapGroup"] = map_group
                headers["data"][shared_label]["mapNum"] = map_num

                version = "FIRERED"
                if "LeafGreen" in shared_label:
                    version = "LEAFGREEN"
                self.WriteLine(f"#ifdef {version}")
                for mon_type in mon_types:
                    if mon_type not in map_encounters:
                        headers["data"][shared_label][mon_type] = "NULL"
                        continue
                    
                    mons_entry = map_encounters[mon_type]
                    encounter_rate = mons_entry["encounter_rate"]
                    mons = mons_entry["mons"]

                    mon_array_name = base_label + "_" + mon_type.title().replace("_", "")
                    self.WriteMonInfos(mon_array_name, mons, encounter_rate)
                    headers["data"][shared_label][season][time][mon_type] = mon_array_name + "Info"
                self.WriteLine(f"#endif")

            self.WritePokemonHeaders(headers)


def ConvertToHeaderFile(json_data):
    with open('src/data/wild_encounters.h', 'w') as output_file:
        config = Config('include/config/overworld.h')
        assembler = WildEncounterAssembler(output_file, json_data, config)
        assembler.WriteHeader()
        assembler.WriteMacros()
        assembler.WriteEncounters()

def main():
    with open('src/data/wild_encounters.json', 'r') as json_file:
        json_data = json.load(json_file)
        ConvertToHeaderFile(json_data)
        

if __name__ == '__main__':
    main()
