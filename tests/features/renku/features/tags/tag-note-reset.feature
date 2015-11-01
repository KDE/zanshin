Feature: Tag reset
  As someone using notes
  I can reset a note from all its tags
  In order to maintain the semantic

  Scenario: Resetting a note dissociate it from all its tags
    Given I display the "Tags / Physics" page
    And there is an item named "A note about nothing interesting" in the central list
    When I drop the item on "Inbox" in the page list
    And I list the items
    And the list does not contain "A note about nothing interesting"
    And I display the "Inbox" page
    And I look at the central list
    And I list the items
    Then the list is:
       | display                          |
       | A note about nothing interesting |
       | A note about philosophy          |
       | A random note on life            |

