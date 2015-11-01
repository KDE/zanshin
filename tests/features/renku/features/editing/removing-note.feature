Feature: Removing notes
  As a note junkie
  I can delete a note so it is removed
  In order to clean up the old junk I accumulated

  Scenario Outline: Removing a note from a page
    Given I display the "<page>" page
    And there is an item named "<title>" in the central list
    When I remove the item
    And I list the items
    Then the list does not contain "<title>"

  Examples:
    | page               | title                 |
    | Inbox              | A page of diary       |
    | Tags / Philosophy  | A random note on life |

