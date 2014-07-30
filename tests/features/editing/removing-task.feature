Feature: Removing tasks
  As a task junkie
  I can delete a task so it is removed
  In order to clean up the old junk I accumulated

  Scenario: Removing a simple task from the inbox
    Given I display the inbox page
    And there is an item named "Buy a book" in the central list
    When I remove the item
    And I list the items
    Then the list does not contain "Buy a book"

