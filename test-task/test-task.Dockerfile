FROM python:3

COPY . .

CMD [ "python", "./run.py" ]
