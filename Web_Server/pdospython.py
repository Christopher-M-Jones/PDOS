from flask import Flask, render_template #, request
import time
from time import sleep
import requests
import json
import math

app = Flask('myproject')

thingspeakAPIKey = "" # API Key redacted

api_url_weather = "https://api.thingspeak.com/channels/1564946/fields/1.json?api_key=" + "&results=2"
api_url_vars = "https://api.thingspeak.com/channels/1564946/fields/2.json?api_key=" + "&results=2"

def get_weather():

    print("Getting weather data....\n\n")
    weather_gather = requests.get(api_url_weather)

    try:
        print("Parsing weather data....")

        weather_json = weather_gather.json()
        weather_data = weather_json['feeds']
        weather_data_values = weather_data[0]
        weather_data_necessary_items = weather_data_values['field1']
        print("\n\n", weather_data_necessary_items)

        weather_results = json.loads(weather_data_necessary_items)
        temperature = weather_results['temp']
        temperature = float(temperature)
        fahr_temp_backup = temperature

        return temperature

    except:
        pass



def get_arduino():

    print("Getting Arduino values....\n\n")
    arduino_gather = requests.get(api_url_vars)
    print(arduino_gather)

    try:
        print("Parsing Arduino data....")
        arduino_json = arduino_gather.json()

        arduino_data = arduino_json['feeds']
        arduino_data_values = arduino_data[0]
        arduino_data_necessary_items = arduino_data_values['field2']
        print("\n\n", arduino_data_necessary_items)

        arduino_results = json.loads(arduino_data_necessary_items)
        print("\n\n", arduino_results)

        variables = []
        slotone = arduino_results['Power Slot One']
        slottwo = arduino_results['Power Slot Two']
        battery = arduino_results['State of Charge']
        grid = arduino_results['Grid Price']
        solar = arduino_results['Solar Irradiance']

        battery = float(battery)
        grid = float(grid)
        solar = float(solar)
        slottwo = int(slottwo)
        slotone = int(slotone)

        variables.append(slotone)
        variables.append(slottwo)
        variables.append(battery)
        variables.append(grid)
        variables.append(solar)

        return variables

    except:
        pass


#Initial startup values
battery_percentage_backup = 0
grid_output_backup = 0
solar_output_backup = 0
slot1_backup = 0
slot2_backup = 0
fahr_temp_backup = 0


@app.route("/")
@app.route("/home")
def home():
        return render_template('home.html', title = 'Home Menu Page')

#routes to the battery page
@app.route("/battery")
def battery():

    global battery_percentage_backup

    arduino_variables = get_arduino()

    if(arduino_variables == None):
        battery_percentage = battery_percentage_backup
    else:
        battery = arduino_variables[2]
        battery_percentage = ((battery - 2.75)/ (4.0- 2.75)) * 100
        battery_percentage = round(battery_percentage, 2)
        battery_percentage_backup = battery_percentage

    return render_template('battery.html', title = 'Battery Page', battery_percentage = battery_percentage)

#routes to the grid page
@app.route("/grid")
def grid():

    global grid_output_backup

    arduino_variables = get_arduino()

    if(arduino_variables == None):
        grid_output = grid_output_backup
    else:
        grid_output = arduino_variables[3]
        grid_output_backup = grid_output

    return render_template('grid.html', title = 'Grid Page', grid_output = grid_output)

#routes to the solar page
@app.route("/solar")
def solar():

    global solar_output_backup

    arduino_variables = get_arduino()

    if(arduino_variables == None):
        solar_output = solar_output_backup
    else:
        solar_output = arduino_variables[4]
        solar_output = solar_output / 0.00074
        solar_output = math.trunc(solar_output)
        solar_output_backup = solar_output

    return render_template('solar.html', title = 'Solar Power Page', solar_output = solar_output)

#routes to the power slots page
@app.route("/powerslots")
def powerslots():

    global slot1_backup
    global slot2_backup

    arduino_variables = get_arduino()

    if(arduino_variables == None):
        slot1 = slot1_backup
        slot2 = slot2_backup
    else:
        slot1 = arduino_variables[0]
        slot2 = arduino_variables[1]
        slot1_backup = slot1
        slot2_backup = slot2

    return render_template('powerslots.html', title = 'Power Slots Page', slot1 = slot1, slot2 = slot2)


#routes to the temperature page
@app.route("/temp")
def temp():

    global fahr_temp_backup

    temp = get_weather()

    if(temp == None):
        fahr_temp = fahr_temp_backup
    else:
        fahr_temp = ((temp) - 273.15) * (9/5) + 32
        fahr_temp = math.trunc(fahr_temp)
        fahr_temp_backup = fahr_temp

    return render_template('temp.html', title = 'Temperature Page', fahr_temp = fahr_temp)


if __name__ == '__main__':
        app.run(debug = True)
