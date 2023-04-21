from flask import Flask, request
import firebase_admin
from firebase_admin import credentials
from firebase_admin import db
from time import sleep

cred = credentials.Certificate('firebaseConfig.json')
firebase_admin.initialize_app(cred,{'databaseURL' : 'https://edith-smarthome-default-rtdb.firebaseio.com/'})

app = Flask(__name__)

@app.route('/webhook', methods = ['POST'])
def webHook():
    req = request.get_json(force=True)
    print(req)
    print(req['queryResult']['parameters']['powerfunctions'])
    state = ''
    # if req['queryResult']['parameters']['powerfunctions'] == "1":
    #     state = 'Turned On'
    # else:
    #     state = 'Turned Off'
    temperature = ['ac', 'temperature', 'degree', 'degrees', 'feels', 'humidity']
    lights = ['light', 'bulb', 'glow']
    fans = ['fans', 'fan']
    roomFeels = ['heat', 'cold']
    if req['queryResult']['parameters']['RoomFeeling'].lower() in roomFeels:
        print(req['queryResult']['parameters']['RoomFeeling'].lower())
        db.reference('/').update({
            'fans' : False if req['queryResult']['parameters']['RoomFeeling'].lower() == 'heat' else True
        })
        return {
            'fulfillmentText' : req['fulfillmentText'] + '. Turning on Fans' if req['queryResult']['parameters']['RoomFeeling'].lower() == 'heat' else '. Turning off Fans'
        }
    if req['queryResult']['parameters']['devices'].lower() in fans:
        powerStat    = False if req['queryResult']['parameters']['powerfunctions'] == "1" else True
        resp = "Turned on" if powerStat == False else "Turned off"
        db.reference('/').update({
            'fans' : powerStat
        })
        return{
            'fulfillmentText' : f"Fan is " + resp
        }
    if req['queryResult']['parameters']['devices'].lower() in lights:
        powerStat    = False if req['queryResult']['parameters']['powerfunctions'] == "1" else True
        resp = "Turned on" if powerStat == False else "Turned off"
        db.reference('/').update({
            'lights' : powerStat
        })
        return{
            'fulfillmentText' : f"Light is " + resp
        }
    detectDevices = ['ac', 'fan', 'temperature', 'degree']
    queryDetection = ['what is', 'how much']
    if str(req['queryResult']['parameters']['devices']).lower() in detectDevices:
        if str(req['queryResult']['parameters']['queries']).lower() in queryDetection:
            db.reference('/').update({
                'toReadTemp' : True
            })
            ans = db.reference('/temperature').get()
            print(ans)
            db.reference('/').update({
                'toReadTemp' : False
            })
            return {
                'fulfillmentText' : f'The Temperature is {ans}° Celcius'
            }

    return{
        'fulfillmentText' : f"{req['queryResult']['parameters']['devices']} is {state}"
    }

# def readDb(path):
def powerFunction(keys, value):
    db.reference('/').update({
        'fans' : value
    })

if __name__ == '__main__':
    app.secret_key = 'SecretKey'
    app.debug = True
    app.run(port=5000)    