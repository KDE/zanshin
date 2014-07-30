Feature: Removing tasks
  As a task junkie
  I can delete a task so it is removed
  In order to clean up the old junk I accumulated

  Scenario: Removing a simple task from the inbox
  Given I'm looking at the inbox view
  And an item named "Buy a book" in the central list
  When I remove the item
  And I look at the central list
  Then The list does not contain "Buy a book"

