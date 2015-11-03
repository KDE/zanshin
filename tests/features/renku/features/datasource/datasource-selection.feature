Feature: Data sources selection
  As an advanced user
  I can select or deselect sources
  In order to see more or less content

  Scenario: Unchecking impacts the inbox
    Given I display the "Inbox" page
    And there is an item named "TestData / Emails / Notes" in the available data sources
    When I uncheck the item
    And I look at the central list
    And I list the items
    Then the list is:
       | display                          |

  Scenario: Checking impacts the inbox
    Given I display the "Inbox" page
    And there is an item named "TestData / Emails / Notes" in the available data sources
    When I check the item
    And I look at the central list
    And I list the items
    Then the list is:
       | display                          |
       | A note about nothing interesting |

