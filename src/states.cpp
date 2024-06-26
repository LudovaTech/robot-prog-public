#include "states.h"

FieldProperties::FieldProperties(
    float fieldLength,
    float fieldWidth,
    float spaceBeforeLineSide,
    float goalWidth,
    Vector2 myGoalPos,
    Vector2 enemyGoalPos,
    float robotRadius,
    float ballRadius)
    : _fieldLength(fieldLength),
      _fieldWidth(fieldWidth),
      _spaceBeforeLineSide(spaceBeforeLineSide),
      _goalWidth(goalWidth),
      _myGoalPos(myGoalPos),
      _enemyGoalPos(enemyGoalPos),
      _robotRadius(robotRadius),
      _ballRadius(ballRadius) {}

ReadingData::ReadingData() {}

void ReadingData::nowWriteInYState() {
  _writingInXState = false;
}

void ReadingData::addToActiveReadingState(char newChar) {
  if (writingInXState()) {
    _xReadingState += newChar;
  } else {
    _yReadingState += newChar;
  }
}

void ReadingData::reinitWith(char newChar) {
  _typeState = newChar;
  _xReadingState = "";
  _yReadingState = "";
  _writingInXState = true;
}

String ReadingData::toString() const {
  String s = "(ReadingData, ";
  s += typeState();
  s += ", ";
  s += xReadingState();
  s += ", ";
  s += yReadingState();
  s += ", ";
  s += writingInXState();
  s += ")";
  return s;
}

RobotState::RobotState(
    Vector2 ballPos,
    Vector2 myPos,
    Vector2 partnerPos,
    Vector2 myGoalPos,
    Vector2 enemyGoalPos)
    : _ballPos(ballPos),
      _myPos(myPos),
      _partnerPos(partnerPos),
      _myGoalPos(myGoalPos),
      _enemyGoalPos(enemyGoalPos) {}

bool RobotState::updateFromString(ReadingData readingData, char newChar) {
  if (newChar == 'b' || newChar == 'm' || newChar == 'p' || newChar == 'g' || newChar == 'G') {
    if (readingData.xReadingState() != "" && readingData.yReadingState() != "") {
      MutableVector2 newMutableVector2 = MutableVector2(Vector2(
          readingData.xReadingState().toFloat(),
          readingData.yReadingState().toFloat()));

      // SerialDebug.println("change to " + newMutableVector2.toString());
      switch (readingData.typeState()) {
        case 'b':
          _ballPos = newMutableVector2;
          break;
        case 'm':
          _myPos = newMutableVector2;
          break;
        case 'p':
          _partnerPos = newMutableVector2;
          break;
        case 'g':
          _myGoalPos = newMutableVector2;
          break;
        case 'G':
          _enemyGoalPos = newMutableVector2;
          break;
        default:
          SerialDebug.println("ERROR CATCHED RobotState: unfinished data : '" + readingData.xReadingState() + " , " + readingData.yReadingState() + "'");
      }
      readingData.reinitWith(newChar);
      return true;
    } else if (!(readingData.typeState() == 'b' || readingData.typeState() == 'm' || readingData.typeState() == 'p' || readingData.typeState() == 'g' || readingData.typeState() == 'G')) {
      SerialDebug.println("ERROR CATCHED RobotState: no typeState tracked");
    } else if (isDigit(newChar) || newChar == '.' || newChar == '-') {
      readingData.addToActiveReadingState(newChar);
    } else if (newChar == ',') {
      if (readingData.writingInXState()) {
        readingData.nowWriteInYState();
      } else {
        SerialDebug.println("ERROR CATCHED RobotState : several characters ','");
        readingData.reinitWith('x');
      }
    } else {
      SerialDebug.println("ERROR CATCHED RobotState : unknown char (skipped) '" + String(int(newChar)) + "'");
    }
  }
  return false;
}

String RobotState::toString() const {
  String result = "RobotState (ballPos: ";
  result += _ballPos.toString();
  result += " myPos: ";
  result += _myPos.toString();
  result += " partnerPos: ";
  result += _partnerPos.toString();
  result += " myGoalPos: ";
  result += _myGoalPos.toString();
  result += " enemyGoalPos: ";
  result += _enemyGoalPos.toString();
  result += ")";
  return result;
}