Feature: Adding tasks
  As a task junkie
  I can create task by giving a title
  In order to collect ideas while reflecting on my life

  Scenario: Adding a task in the inbox
  Given I'm looking at the inbox view
  When I add a task named "Buy a book"
  And I look at the central list
  Then The list contains "Buy a book"

