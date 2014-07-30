Feature: Adding tasks
  As a task junkie
  I can create task by giving a title
  In order to collect ideas while reflecting on my life

  Scenario: Adding a task in the inbox
    Given I display the inbox page
    And I look at the central list
    When I add a task named "Buy a book"
    And I list the items
    Then the list contains "Buy a book"

