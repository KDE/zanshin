Feature: Removing tasks
  As a task junkie
  I can delete a task so it is removed
  In order to clean up the old junk I accumulated

  Scenario Outline: Removing a simple task from a page
    Given I display the "<page>" page
    And there is an item named "<title>" in the central list
    When I remove the item
    And I list the items
    Then the list does not contain "<title>"

  Examples:
    | page               | title                 |
    | Inbox              | Buy a book            |
    | Projects / Backlog | Setup a release party |

