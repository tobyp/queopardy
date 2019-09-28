# Queopardy

Jeopardy in Qt using QtQuick/QML

## Usage

```
./queopardy board.json
```

All paths mentioned in the board file are resolved relative to the file itself.

## Attribution

The file `duck.jpg` is licensed from the original publisher Eva Rinaldi under the terms of CC BY-SA 2.0 [1].

[1] https://www.flickr.com/photos/evarinaldiphotography/8374802487/

## Protocol

Queopardy includes a text-based WebSocket server for remote buzzers to connect to.

| Sender | Syntax | Description |
|--------|--------|-------------|
| Client | `hello <name...>` | Connect as player `name`. If the player exists and is not already controlled by another client, you will get control of it. If it doesn't exist and the game is not started yet, it is created and assigned to you. Otherwise you receive an error. |
| Server | `ok <message...>` | Something succeeded. Mostly for debugging. |
| Server | `error <message...>` | Something went wrong. Mostly for debugging. |
| Server | `welcome` | Response to a successful `hello` command. |
| Client | `color <r> <g> <b>` | Set your player color (r, g, b are unsigned byte values). |
| Client | `nextColor` | Automatically get the next free pre-defined color |
| Server | `question open <question...>` | A question has been selected. Everything after `open ` is the question text. Image questions only send `<image>`. |
| Server | `buzzer open` | It is now allowed to buzz. |
| Client | `buzz` | Try to buzz. |
| Server | `buzzer closed <name...>` | Someone buzzed, so you can no longer buzz yourself. |
| Server | `buzzer closed` | The buzzer was closed because the question was cancelled. |
| Server | `question closed` | The question was closed. |