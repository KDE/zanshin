Feature: Tag note dissociation
  As someone collecting notes
  I can delete a note related to a tag
  In order to keep my tag meaningful

  Scenario: Removing a note linked to a tag from the central list
    Given I display the "Inbox" page
    And I look at the central list
    Then the list is:
       | display          |
    And I display the "Tags / Philosophy" page
    And there is an item named "A note about philosophy" in the central list
    When I remove the item
    And I look at the central list
    Then the list does not contain "A note about philosophy"
