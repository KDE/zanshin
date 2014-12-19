Feature: Tag task dissociation
  As someone collecting tasks and notes
  I can delete a task related to a tag
  In order to keep my tag meaningful

  Scenario: Removing a task linked to a tag from the central list
    Given I display the "Inbox" page
    And I look at the central list
    Then the list is:
       | display          |
    And I display the "Tags / Philosophy" page
    And there is an item named ""Capital in the Twenty-First Century" by Thomas Piketty" in the central list
    When I remove the item
    And I look at the central list
    Then the list does not contain ""Capital in the Twenty-First Century" by Thomas Piketty"
