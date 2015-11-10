Feature: Data sources listing
  As an advanced user
  I can list sources
  In order to list and store notes

  Scenario: All note sources appear in the list
    Given I display the available data sources
    When I list the items
    Then the list is:
       | display                                      | icon                |
       | TestData                                     | folder              |
       | TestData / Emails                            | folder              |
       | TestData / Emails / Notes                    | folder              |
       | TestData / Private Notes                     | folder              |
