Feature: Adding tasks
  As a task junkie
  I can create task by giving a title
  In order to collect ideas while reflecting on my life

  Scenario Outline: Adding a task in a page
    Given I display the "<page>" page
    And I look at the central list
    When I add a task named "<title>"
    And I list the items
    Then the list contains "<title>"

  Examples:
    | page               | title                 |
    | Inbox              | Buy a book            |
    | Projects / Backlog | Setup a release party |
