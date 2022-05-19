import argparse
import json
import time

# python3 run.py --settings '{"sleep": 30}' --options '{"log-string": "Hello, world!"}'

parser = argparse.ArgumentParser(description='Sleep.')
parser.add_argument('--settings', help='EXAMPLE: {"sleep": 30}')
parser.add_argument('--options', help='EXAMPLE: {"log-string": "Hello, world!"}')

args = parser.parse_args()
settings = json.loads(args.settings)
options = json.loads(args.options)
print(options.get('log-string', 'Where is log string?'))

sleep_duration = int(settings.get('sleep', '0'))
print(f"Sleep for {sleep_duration} seconds...")
time.sleep(sleep_duration)

print(options.get('log-string', 'Where is log string?'))
