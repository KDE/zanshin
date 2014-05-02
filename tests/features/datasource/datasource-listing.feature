Feature: Data sources listing
  As an advanced user
  I can list sources
  In order to list and store artefaces

  Scenario: Flat task data sources
    Given I got a task data source list model
    When I list the model
    Then the list is:
       | display                                |
       | TestData/Calendar1                     |
       | TestData/Calendar1/Calendar2           |
       | TestData/Calendar1/Calendar2/Calendar3 |

  Scenario: Flat note data sources
    Given I got a note data source list model
    When I list the model
    Then the list is:
       | display                                |
       | TestData/Emails/Notes                  |
