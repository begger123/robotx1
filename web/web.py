import serial

class RobotDriver():

    def __init__(self):
        self.serial = serial.Serial('/dev/ttyAMA0', 9600, timeout=0.1)
        self.clear_line()
        print 'Ready.'

    def clear_line(self):
        while self.serial.readline() != '':
            pass

    def get_battery(self):
        self.serial.write('B')
        self.serial.readline()
        return (self.serial.readline() + self.serial.readline()).strip()

    @staticmethod
    def _speed_to_4char(speed):
        return str('%s%d' % ('+' if speed >= 0 else '-', abs(speed))).zfill(4)

    def set_speed(self, left_speed, right_speed):
        self.serial.write('S%s%s' % (self._speed_to_4char(-left_speed),
                                     self._speed_to_4char(right_speed)))


import flask
app = flask.Flask('RobotX1')
driver = RobotDriver()

@app.route('/')
def web_main():
    return flask.send_from_directory('', 'webui.html')

@app.route('/battery')
def web_battery():
    driver.clear_line();
    return driver.get_battery().replace('\n', '<br>')

@app.route('/speed/<left>,<right>')
def web_speed(left, right):
    driver.set_speed(int(left), int(right))
    return 'OK'

if __name__ == '__main__':
    app.run(host='0.0.0.0', threaded=True, debug=True)
