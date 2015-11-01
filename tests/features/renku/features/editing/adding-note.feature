Feature: Adding notes
  As a note junkie
  I can create note by giving a title
  In order to collect ideas while reflecting on my life

  Scenario Outline: Adding a note in a page
    Given I display the "<page>" page
    And I look at the central list
    When I add a "note" named "<title>"
    And I list the items
    Then the list contains "<title>"

  Examples:
    | page               | title                 |
    | Inbox              | A page of diary       |
    | Tags / Philosophy  | A random note on life |
